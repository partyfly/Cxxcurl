#include "Cxxcurl.h"
#include <unistd.h>
int main() {
	auto func = [](char* data, size_t len) {
		printf("%d\n",len);
	};
	CXXCURL::Get("www.example.com", func);
	CXXCURL::Post("www.example.com", "123",func);
	while(true) {
		sleep(1);
	}
	return 0;
}
