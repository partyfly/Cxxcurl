#ifndef __CXXCURl_H__
#define __CXXCURL_H__
#include <curl/curl.h>
#include <string>
#include <thread>
#include <mutex>
#include <functional>
#include <string.h>

namespace CXXCURL {
using namespace std;
extern recursive_mutex mtx;
extern atomic_flag working;

struct ContextBase {
	ContextBase():len_(0) {}
	void (*typeFunc_)(void* self);
	char buff_[20000];
	size_t len_;
};
template<typename type>
struct Context : public ContextBase {
	Context(type& userFunc):func_(userFunc) {}
	type func_;
};

template<typename type>
size_t write_callback(char *ptr, size_t size, size_t nmemb, Context<type> *userdata) {
	memcpy(userdata->buff_ + userdata->len_, ptr, size*nmemb);
	userdata->len_ += size*nmemb;
	return size*nmemb;
}

template<typename type>
void WriteDone(void* data) {
	Context<type>* self = (Context<type>*)data;
	(self->func_)(self->buff_, self->len_);
	delete self;
}

CURLM* GetDefaultMHandle();

void CurlThread(CURLM* curlm);

template<typename type>
string& Get(const char* url, type& cb) {
	Context<type>* ctx = new Context<type>(cb);
	ctx->typeFunc_ = (void (*)(void* self))((void*)WriteDone<type>);
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, ctx);
	curl_easy_setopt(curl, CURLOPT_PRIVATE, ctx);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback<type>);
	//curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2);
	//unique_lock<recursive_mutex> lk(mtx, std::defer_lock);
	mtx.lock();
	curl_multi_add_handle(GetDefaultMHandle(), curl);
	mtx.unlock();
	if (working.test_and_set()) {
		thread worker(CurlThread, GetDefaultMHandle());
		worker.detach();
	}
}

template<typename type>
string& Post(const char* url, const char* body, type& cb, bool selfHelp = false) {
	Context<type>* ctx = new Context<type>(cb);
	ctx->typeFunc_ = (void (*)(void* self))((void*)WriteDone<type>);
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, ctx);
	curl_easy_setopt(curl, CURLOPT_PRIVATE, ctx);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback<type>);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	if (selfHelp) 
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	else
		curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, body);
	//curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2);
	//unique_lock<mutex> lk(mtx, std::defer_lock);
	mtx.lock();
	curl_multi_add_handle(GetDefaultMHandle(), curl);
	mtx.unlock();
	if (working.test_and_set()) {
		thread worker(CurlThread, GetDefaultMHandle());
		worker.detach();
	}
}

}

#endif
