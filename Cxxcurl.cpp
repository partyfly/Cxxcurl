#include "Cxxcurl.h"
#include <unistd.h>
#include <assert.h>

namespace CXXCURL {
using namespace std;

recursive_mutex mtx;
atomic_flag working = ATOMIC_FLAG_INIT;

void CurlThread(CURLM* curlm) {
	int still_running = 1;
	unique_lock<recursive_mutex> lk(mtx, std::defer_lock);
	do {
		struct timeval timeout;
		long timel = -1;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		curl_multi_timeout(curlm, &timel);
		if(timel >= 0) {
			timeout.tv_sec = timel / 1000;
			timeout.tv_usec = (timel % 1000) * 1000;
		}

		fd_set fdread,fdwrite,fdexcep;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);
		int maxfd = -1;
		lk.lock();
		CURLMcode mc = curl_multi_fdset(curlm, &fdread, &fdwrite, &fdexcep, &maxfd);
		if(mc != CURLM_OK) {
			fprintf(stderr, "fdset error, code %d.\n", mc);
			break;
		}
		int rc;
		if(maxfd == -1) {
			struct timeval wait = { 0, 100 * 1000 }; /* 100ms */ 
			rc = select(0, NULL, NULL, NULL, &wait);
		}else {
			rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
		}
		switch(rc) {
			case -1: {
				still_running = 0;
				printf("select() returns error\n");
			}
			break;
			case 0:
			default:
			{
				curl_multi_perform(curlm, &still_running);
			}
			break;
		}
		int msgq = 0;
		struct CURLMsg* m;
		while(m = curl_multi_info_read(curlm, &msgq)) {
			if (m->msg == CURLMSG_DONE) {
				ContextBase* ctxBase;
				curl_easy_getinfo(m->easy_handle, CURLINFO_PRIVATE, &ctxBase);
				(*ctxBase->typeFunc_)(ctxBase);
				curl_multi_remove_handle(curlm, m->easy_handle);
				curl_easy_cleanup(m->easy_handle);
			}
		}
		lk.unlock();
		usleep(1);
	}while(true);
}

CURLM* GetDefaultMHandle() {
	static CURLM* curlm = nullptr;
	if (curlm == nullptr) {
		curl_global_init(CURL_GLOBAL_ALL);
		curlm = curl_multi_init();
	}
	return curlm;
}

}
